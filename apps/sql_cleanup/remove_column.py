import sys
import re

def remove_column(sql, column):
    # Split the SQL statement into parts
    parts = sql.split("VALUES")
    columns_part = parts[0]
    values_part = parts[1]

    insert_part = columns_part.split("(")[0]
    columns_part = columns_part.split("(")[1]

    # Remove `(` and `)` from columns and values parts
    columns_part = re.sub(r'[()]', '', columns_part)
    values_part = re.sub(r'[()]', '', values_part)
    
    # trim empty spaces from the front and back of values_part
    values_part = values_part.strip()

    # if the last character of values_part is a semicolon, remove it
    if values_part[-1] == ";":
        values_part = values_part[:-1]

    # Split the columns and values into lists
    columns = columns_part.split(", ")
    #columns.pop(0)
    values = values_part.split(", ")

    # Try to find and remove the specified column and its corresponding value
    try:
        index = columns.index(f"`{column}`")
        del columns[index]
        del values[index]
    except ValueError:
        # Column not found, return the original SQL
        return sql

    # concatenate a ( onto the end of the insert part of the query
    insert_part = f"{insert_part}("

    # Recreate the SQL without the specified column and its value 
    new_columns_part = ", ".join(columns)
    new_values_part = ", ".join(values)
    new_sql = f"{insert_part}{new_columns_part}) VALUES ({new_values_part});"
 
    return new_sql

# get an array of all of the parameters to the script

args = sys.argv
# pop the name of the script off the args
args.pop(0)


# Read from stdin (piped input)
for line in sys.stdin:
    # loop over args and call remove_columnn for each arg
    for arg in args:
        line = remove_column(line, arg)
    print(line)

