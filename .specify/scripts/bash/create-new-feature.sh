#!/usr/bin/env bash

set -e

JSON_MODE=false
DRY_RUN=false
ALLOW_EXISTING=false
SHORT_NAME=""
BRANCH_NUMBER=""
USE_TIMESTAMP=false
FEATURE_DIRECTORY=""
REQUESTED_BRANCH=""
# Project integration baseline; override explicitly for another local baseline.
BASE_BRANCH="${SPECIFY_BASE_BRANCH:-main}"
ARGS=()
i=1
while [ $i -le $# ]; do
    arg="${!i}"
    case "$arg" in
        --json)
            JSON_MODE=true
            ;;
        --dry-run)
            DRY_RUN=true
            ;;
        --allow-existing-branch)
            ALLOW_EXISTING=true
            ;;
        --feature-directory|--branch-name)
            if [ $((i + 1)) -gt $# ]; then
                echo "Error: $arg requires a value" >&2
                exit 1
            fi
            i=$((i + 1))
            next_arg="${!i}"
            if [[ -z "$next_arg" || "$next_arg" == --* ]]; then
                echo "Error: $arg requires a value" >&2
                exit 1
            fi
            if [ "$arg" = "--feature-directory" ]; then
                FEATURE_DIRECTORY="$next_arg"
            else
                REQUESTED_BRANCH="$next_arg"
            fi
            ;;
        --short-name)
            if [ $((i + 1)) -gt $# ]; then
                echo 'Error: --short-name requires a value' >&2
                exit 1
            fi
            i=$((i + 1))
            next_arg="${!i}"
            # Check if the next argument is another option (starts with --)
            if [[ "$next_arg" == --* ]]; then
                echo 'Error: --short-name requires a value' >&2
                exit 1
            fi
            SHORT_NAME="$next_arg"
            ;;
        --number)
            if [ $((i + 1)) -gt $# ]; then
                echo 'Error: --number requires a value' >&2
                exit 1
            fi
            i=$((i + 1))
            next_arg="${!i}"
            if [[ "$next_arg" == --* ]]; then
                echo 'Error: --number requires a value' >&2
                exit 1
            fi
            BRANCH_NUMBER="$next_arg"
            ;;
        --timestamp)
            USE_TIMESTAMP=true
            ;;
        --help|-h)
            echo "Usage: $0 [--json] [--dry-run] [--allow-existing-branch] [--feature-directory <path>] [--branch-name <name>] [--short-name <name>] [--number N] [--timestamp] <feature_description>"
            echo ""
            echo "Options:"
            echo "  --json              Output in JSON format"
            echo "  --dry-run           Validate and preview without switching branches or writing files"
            echo "  --allow-existing-branch  Reuse an existing branch/directory without overwriting spec.md"
            echo "  --feature-directory <path>  Override SPECIFY_FEATURE_DIRECTORY and feature.json"
            echo "  --branch-name <name>  Actual Git branch (default: selected directory basename)"
            echo "  --short-name <name> Provide a custom short name (2-4 words) for the feature"
            echo "  --number N          Specify branch number manually (overrides auto-detection)"
            echo "  --timestamp         Use timestamp prefix (YYYYMMDD-HHMMSS) instead of sequential numbering"
            echo "  --help, -h          Show this help message"
            echo "  SPECIFY_BASE_BRANCH  Existing local baseline branch (default: main)"
            echo ""
            echo "Examples:"
            echo "  $0 'Add user authentication system' --short-name 'user-auth'"
            echo "  $0 'Implement OAuth2 integration for API' --number 5"
            echo "  $0 --timestamp --short-name 'user-auth' 'Add user authentication'"
            echo "  $0 --feature-directory specs/01-foundation --json 'Foundation iteration'"
            exit 0
            ;;
        --*)
            echo "Error: Unknown option: $arg" >&2
            exit 1
            ;;
        *)
            ARGS+=("$arg")
            ;;
    esac
    i=$((i + 1))
done

FEATURE_DESCRIPTION="${ARGS[*]}"
if [ -z "$FEATURE_DESCRIPTION" ]; then
    echo "Usage: $0 [--json] [--dry-run] [--allow-existing-branch] [--short-name <name>] [--number N] [--timestamp] <feature_description>" >&2
    exit 1
fi

# Trim whitespace and validate description is not empty (e.g., user passed only whitespace)
FEATURE_DESCRIPTION=$(echo "$FEATURE_DESCRIPTION" | sed -E 's/^[[:space:]]+|[[:space:]]+$//g')
if [ -z "$FEATURE_DESCRIPTION" ]; then
    echo "Error: Feature description cannot be empty or contain only whitespace" >&2
    exit 1
fi

# Function to get highest number from specs directory
get_highest_from_specs() {
    local specs_dir="$1"
    local highest=0

    if [ -d "$specs_dir" ]; then
        for dir in "$specs_dir"/*; do
            [ -d "$dir" ] || continue
            dirname=$(basename "$dir")
            # Match sequential prefixes (>=3 digits), but skip timestamp dirs.
            if echo "$dirname" | grep -Eq '^[0-9]{3,}-' && ! echo "$dirname" | grep -Eq '^[0-9]{8}-[0-9]{6}-'; then
                number=$(echo "$dirname" | grep -Eo '^[0-9]+')
                number=$((10#$number))
                if [ "$number" -gt "$highest" ]; then
                    highest=$number
                fi
            fi
        done
    fi

    echo "$highest"
}

# Function to clean and format a branch name
clean_branch_name() {
    local name="$1"
    echo "$name" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/-/g' | sed 's/-\+/-/g' | sed 's/^-//' | sed 's/-$//'
}

# Resolve repository root using common.sh functions which prioritize .specify
SCRIPT_DIR="$(CDPATH="" cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

REPO_ROOT=$(get_repo_root) || exit 1

cd "$REPO_ROOT"

SPECS_DIR="$REPO_ROOT/specs"

fail() { echo "Error: $*" >&2; exit 1; }

# Resolve paths without mkdir/realpath (including nonexistent dry-run targets).
# Existing ancestors are resolved physically to prevent symlink escapes from specs/.
normalize_path() {
    local path="$1" part result='' ancestor remainder=''
    local -a parts=()
    [[ "$path" != *$'\n'* && "$path" != *$'\r'* ]] || return 1
    [[ "$path" == /* ]] || path="$REPO_ROOT/$path"
    IFS='/' read -r -a parts <<< "$path"
    for part in "${parts[@]}"; do
        case "$part" in
            ''|.) ;;
            ..) result="${result%/*}" ;;
            *) result="$result/$part" ;;
        esac
    done
    ancestor="${result:-/}"
    while [ ! -d "$ancestor" ]; do
        [[ -e "$ancestor" || -L "$ancestor" ]] && return 1
        remainder="/$(basename "$ancestor")$remainder"
        ancestor="$(dirname "$ancestor")"
    done
    ancestor="$(CDPATH='' cd -- "$ancestor" && pwd -P)" || return 1
    printf '%s\n' "${ancestor%/}$remainder"
}

[[ "$(git rev-parse --is-inside-work-tree 2>/dev/null)" = true ]] || fail 'A Git working tree is required.'
git show-ref --verify --quiet "refs/heads/$BASE_BRANCH" || fail "Integration branch '$BASE_BRANCH' does not exist."
CURRENT_BRANCH=$(git symbolic-ref --quiet --short HEAD) || fail 'Detached HEAD; select the integration or iteration branch explicitly.'
STORED_DIRECTORY=$(read_feature_json_feature_directory "$REPO_ROOT")
if [ -z "$FEATURE_DIRECTORY" ]; then
    FEATURE_DIRECTORY="${SPECIFY_FEATURE_DIRECTORY:-$STORED_DIRECTORY}"
fi
if [[ "$FEATURE_DIRECTORY" == *'$'* || "$REQUESTED_BRANCH" == *'$'* || "$SHORT_NAME" == *'$'* || "$FEATURE_DESCRIPTION" == '$FEATURE_DESCRIPTION' ]]; then
    fail 'Unresolved hook placeholders; supply concrete inputs or export the hook variables.'
fi

# Function to generate branch name with stop word filtering and length filtering
generate_branch_name() {
    local description="$1"

    # Common stop words to filter out
    local stop_words="^(i|a|an|the|to|for|of|in|on|at|by|with|from|is|are|was|were|be|been|being|have|has|had|do|does|did|will|would|should|could|can|may|might|must|shall|this|that|these|those|my|your|our|their|want|need|add|get|set)$"

    # Convert to lowercase and split into words
    local clean_name=$(printf '%s' "$description" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/ /g')

    # Filter words: remove stop words and words shorter than 3 chars (unless they're uppercase acronyms in original)
    local meaningful_words=()
    for word in $clean_name; do
        # Skip empty words
        [ -z "$word" ] && continue

        # Keep words that are NOT stop words AND (length >= 3 OR are potential acronyms)
        if ! echo "$word" | grep -qiE "$stop_words"; then
            if [ ${#word} -ge 3 ]; then
                meaningful_words+=("$word")
            # Keep short words that appear as an uppercase acronym in the original.
            # Uppercase via tr and match with grep -w (both portable) rather than
            # bash's 4+ "^^" case expansion (breaks on macOS bash 3.2) and \b (non-POSIX).
            elif printf '%s' "$description" | grep -qw -- "$(printf '%s' "$word" | tr '[:lower:]' '[:upper:]')"; then
                meaningful_words+=("$word")
            fi
        fi
    done

    # If we have meaningful words, use first 3-4 of them
    if [ ${#meaningful_words[@]} -gt 0 ]; then
        local max_words=3
        if [ ${#meaningful_words[@]} -eq 4 ]; then max_words=4; fi

        local result=""
        local count=0
        for word in "${meaningful_words[@]}"; do
            if [ $count -ge $max_words ]; then break; fi
            if [ -n "$result" ]; then result="$result-"; fi
            result="$result$word"
            count=$((count + 1))
        done
        echo "$result"
    else
        # Fallback to original logic if no meaningful words found
        local cleaned=$(clean_branch_name "$description")
        echo "$cleaned" | tr '-' '\n' | grep -v '^$' | head -3 | tr '\n' '-' | sed 's/-$//'
    fi
}

# Generate branch name
if [ -n "$SHORT_NAME" ]; then
    # Use provided short name, just clean it up
    BRANCH_SUFFIX=$(clean_branch_name "$SHORT_NAME")
else
    # Generate from description with smart filtering
    BRANCH_SUFFIX=$(generate_branch_name "$FEATURE_DESCRIPTION")
fi

# Warn if --number and --timestamp are both specified
if [ "$USE_TIMESTAMP" = true ] && [ -n "$BRANCH_NUMBER" ]; then
    >&2 echo "[specify] Warning: --number is ignored when --timestamp is used"
    BRANCH_NUMBER=""
fi

# Determine branch prefix
if [ -n "$FEATURE_DIRECTORY" ]; then
    FEATURE_DIR=$(normalize_path "$FEATURE_DIRECTORY") || fail 'Invalid feature directory or ancestor.'
    case "$FEATURE_DIR" in
        "$REPO_ROOT"/specs/*) ;;
        *) fail 'Feature directory must be inside this project specs/ directory.' ;;
    esac
    BRANCH_NAME=$(basename "$FEATURE_DIR")
    FEATURE_NUM=''
    if [[ "$BRANCH_NAME" =~ ^([0-9]{8}-[0-9]{6})- ]]; then
        FEATURE_NUM="${BASH_REMATCH[1]}"
    elif [[ "$BRANCH_NAME" =~ ^([0-9]+)- ]]; then
        FEATURE_NUM="${BASH_REMATCH[1]}"
    fi
    # Preserve a previously recorded mapping even when the Git name differs.
    if [ -n "$STORED_DIRECTORY" ]; then
        STORED_ABSOLUTE=$(normalize_path "$STORED_DIRECTORY") || fail 'Invalid persisted feature directory.'
        if [ "$STORED_ABSOLUTE" = "$FEATURE_DIR" ]; then
            STORED_BRANCH=$(read_feature_json_feature_branch "$REPO_ROOT")
            [ -z "$STORED_BRANCH" ] || BRANCH_NAME="$STORED_BRANCH"
            if [[ -n "$REQUESTED_BRANCH" && -n "$STORED_BRANCH" && "$REQUESTED_BRANCH" != "$STORED_BRANCH" ]]; then
                fail "Selected directory is already mapped to branch '$STORED_BRANCH'."
            fi
        fi
    fi
elif [ "$USE_TIMESTAMP" = true ]; then
    FEATURE_NUM=$(date +%Y%m%d-%H%M%S)
    BRANCH_NAME="${FEATURE_NUM}-${BRANCH_SUFFIX}"
else
    # Determine branch number from existing feature directories
    if [ -z "$BRANCH_NUMBER" ]; then
        HIGHEST=$(get_highest_from_specs "$SPECS_DIR")
        BRANCH_NUMBER=$((HIGHEST + 1))
    fi

    # Force base-10 interpretation to prevent octal conversion (e.g., 010 → 8 in octal, but should be 10 in decimal)
    [[ "$BRANCH_NUMBER" =~ ^[0-9]{1,9}$ ]] || fail '--number must be a non-negative integer (at most 9 digits).'
    FEATURE_NUM=$(printf "%03d" "$((10#$BRANCH_NUMBER))")
    BRANCH_NAME="${FEATURE_NUM}-${BRANCH_SUFFIX}"
fi

# GitHub enforces a 244-byte limit on branch names
# Validate and truncate if necessary
MAX_BRANCH_LENGTH=244
if [[ -z "$FEATURE_DIRECTORY" && ${#BRANCH_NAME} -gt $MAX_BRANCH_LENGTH ]]; then
    # Calculate how much we need to trim from suffix
    # Account for prefix length: timestamp (15) + hyphen (1) = 16, or sequential (3) + hyphen (1) = 4
    PREFIX_LENGTH=$(( ${#FEATURE_NUM} + 1 ))
    MAX_SUFFIX_LENGTH=$((MAX_BRANCH_LENGTH - PREFIX_LENGTH))

    # Truncate suffix at word boundary if possible
    TRUNCATED_SUFFIX=$(echo "$BRANCH_SUFFIX" | cut -c1-$MAX_SUFFIX_LENGTH)
    # Remove trailing hyphen if truncation created one
    TRUNCATED_SUFFIX=$(echo "$TRUNCATED_SUFFIX" | sed 's/-$//')

    ORIGINAL_BRANCH_NAME="$BRANCH_NAME"
    BRANCH_NAME="${FEATURE_NUM}-${TRUNCATED_SUFFIX}"

    >&2 echo "[specify] Warning: Branch name exceeded GitHub's 244-byte limit"
    >&2 echo "[specify] Original: $ORIGINAL_BRANCH_NAME (${#ORIGINAL_BRANCH_NAME} bytes)"
    >&2 echo "[specify] Truncated to: $BRANCH_NAME (${#BRANCH_NAME} bytes)"
fi

[ -n "$FEATURE_DIRECTORY" ] || FEATURE_DIR=$(normalize_path "$SPECS_DIR/$BRANCH_NAME") || fail 'Invalid feature directory or ancestor.'
case "$FEATURE_DIR" in
    "$REPO_ROOT"/specs/*) ;;
    *) fail 'Feature directory must be inside this project specs/ directory.' ;;
esac
[ -z "$REQUESTED_BRANCH" ] || BRANCH_NAME="$REQUESTED_BRANCH"
[[ "$BRANCH_NAME" != main && "$BRANCH_NAME" != "$BASE_BRANCH" ]] || fail 'Select an iteration feature branch, not main or the integration branch.'
git check-ref-format "refs/heads/$BRANCH_NAME" >/dev/null 2>&1 && \
    git check-ref-format --branch "$BRANCH_NAME" >/dev/null 2>&1 || fail "Invalid Git branch name: $BRANCH_NAME"
SPEC_FILE="$FEATURE_DIR/spec.md"

BRANCH_EXISTS=false
if git show-ref --verify --quiet "refs/heads/$BRANCH_NAME"; then
    BRANCH_EXISTS=true
fi
if { [ -d "$FEATURE_DIR" ] || [ "$BRANCH_EXISTS" = true ]; } && [ "$ALLOW_EXISTING" != true ]; then
    fail "Branch/directory already exists; use --allow-existing-branch to resume '$BRANCH_NAME'."
fi
[[ ! -e "$SPEC_FILE" || -f "$SPEC_FILE" ]] || fail 'spec.md exists but is not a regular file.'
[[ ! -e "$REPO_ROOT/.specify/feature.json" || -f "$REPO_ROOT/.specify/feature.json" ]] || fail 'feature.json is not a regular file.'
if [ "$CURRENT_BRANCH" != "$BRANCH_NAME" ]; then
    [ -z "$(git status --porcelain)" ] || fail 'Uncommitted work: resolve it before switching branches. No files were changed.'
    if [ "$BRANCH_EXISTS" = false ] && [ "$CURRENT_BRANCH" != "$BASE_BRANCH" ]; then
        fail "Create new iterations from '$BASE_BRANCH', not '$CURRENT_BRANCH'."
    fi
fi
if [ "$BRANCH_EXISTS" = true ]; then
    git merge-base --is-ancestor "$BASE_BRANCH" "$BRANCH_NAME" || fail "Existing branch does not contain the '$BASE_BRANCH' baseline."
fi

# Resolve and retain the template before checkout: an existing branch can have
# different tracked files, including a deleted spec/template. Preserve newlines.
TEMPLATE=$(resolve_template 'spec-template' "$REPO_ROOT") || TEMPLATE=''
TEMPLATE_CONTENT=''
if [ -n "$TEMPLATE" ]; then
    TEMPLATE_CONTENT=$(cat "$TEMPLATE"; printf x)
    TEMPLATE_CONTENT="${TEMPLATE_CONTENT%x}"
fi
if [ ! -f "$SPEC_FILE" ] && [ -z "$TEMPLATE" ]; then
    fail 'Spec template not found.'
fi

if [ "$DRY_RUN" != true ]; then
    if [ "$CURRENT_BRANCH" != "$BRANCH_NAME" ]; then
        if [ "$BRANCH_EXISTS" = true ]; then
            git checkout "$BRANCH_NAME" >&2
        else
            git checkout -b "$BRANCH_NAME" "$BASE_BRANCH" >&2
        fi
    fi
    [[ "$(git symbolic-ref --quiet --short HEAD)" = "$BRANCH_NAME" ]] || fail 'Git did not activate the intended branch.'

    [[ -f "$SPEC_FILE" || -n "$TEMPLATE" ]] || fail 'Spec template not found on the starting branch.'
    mkdir -p "$FEATURE_DIR"

    if [ ! -f "$SPEC_FILE" ]; then
        printf '%s' "$TEMPLATE_CONTENT" > "$SPEC_FILE"
    fi

    # Persist to .specify/feature.json so downstream commands can find the feature
    _persist_feature_json "$REPO_ROOT" "$FEATURE_DIR" "$BRANCH_NAME"

    # Inform the user how to set feature state in their own shell
    printf '# To persist: export SPECIFY_FEATURE=%q\n' "$BRANCH_NAME" >&2
    printf '#              export SPECIFY_FEATURE_DIRECTORY=%q\n' "$FEATURE_DIR" >&2
fi

if $JSON_MODE; then
    if command -v jq >/dev/null 2>&1; then
        if [ "$DRY_RUN" = true ]; then
            jq -cn \
                --arg branch_name "$BRANCH_NAME" \
                --arg spec_file "$SPEC_FILE" \
                --arg feature_dir "$FEATURE_DIR" \
                --arg feature_num "$FEATURE_NUM" \
                '{BRANCH_NAME:$branch_name,FEATURE_DIR:$feature_dir,SPEC_FILE:$spec_file,FEATURE_NUM:$feature_num,DRY_RUN:true}'
        else
            jq -cn \
                --arg branch_name "$BRANCH_NAME" \
                --arg spec_file "$SPEC_FILE" \
                --arg feature_dir "$FEATURE_DIR" \
                --arg feature_num "$FEATURE_NUM" \
                '{BRANCH_NAME:$branch_name,FEATURE_DIR:$feature_dir,SPEC_FILE:$spec_file,FEATURE_NUM:$feature_num}'
        fi
    else
        if [ "$DRY_RUN" = true ]; then
            printf '{"BRANCH_NAME":"%s","FEATURE_DIR":"%s","SPEC_FILE":"%s","FEATURE_NUM":"%s","DRY_RUN":true}\n' "$(json_escape "$BRANCH_NAME")" "$(json_escape "$FEATURE_DIR")" "$(json_escape "$SPEC_FILE")" "$(json_escape "$FEATURE_NUM")"
        else
            printf '{"BRANCH_NAME":"%s","FEATURE_DIR":"%s","SPEC_FILE":"%s","FEATURE_NUM":"%s"}\n' "$(json_escape "$BRANCH_NAME")" "$(json_escape "$FEATURE_DIR")" "$(json_escape "$SPEC_FILE")" "$(json_escape "$FEATURE_NUM")"
        fi
    fi
else
    echo "BRANCH_NAME: $BRANCH_NAME"
    echo "FEATURE_DIR: $FEATURE_DIR"
    echo "SPEC_FILE: $SPEC_FILE"
    echo "FEATURE_NUM: $FEATURE_NUM"
    if [ "$DRY_RUN" != true ]; then
        printf '# To persist in your shell: export SPECIFY_FEATURE=%q\n' "$BRANCH_NAME"
        printf '#                           export SPECIFY_FEATURE_DIRECTORY=%q\n' "$FEATURE_DIR"
    fi
fi
