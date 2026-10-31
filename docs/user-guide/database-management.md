# Database Management

Connect to SQL databases and execute queries directly from DevTools.

## Overview

The Database Management tool provides a graphical interface for connecting to SQL databases, browsing table structures, and executing queries. It supports SQLite, MySQL, and PostgreSQL databases.

## Accessing the Tool

1. Launch DevTools
2. Click **Database** in the sidebar

## Interface Overview

The Database Management interface consists of:

1. **Toolbar**: Shows the active connection and provides **Disconnect**,
   **Refresh**, **Connection settings**, and **New SQL**
2. **Tables** panel (left): Lists tables in the connected database
3. **SQL workspace** (right): Holds one or more query tabs, each with an editor
   and a results table

The initial SQL tab is available before connecting. **Execute** and **Refresh**
remain disabled until a connection is established.

## Connecting to a Database

### Using Connection History

Click **Connection settings** to open the connection dialog:

1. Click **Connect** next to a saved connection in **Recent connections**
2. The dialog closes after the connection succeeds
3. The table list appears in the **Tables** panel

Click **Close** to dismiss the dialog.

### Creating a New Connection

1. Click **Connection settings** in the toolbar
2. Select the **Database type**
3. Fill in the required connection fields (see below)
4. Click **Connect**
5. The dialog closes and the active connection appears in the toolbar

## Supported Databases

| Database | Driver | Connection Type |
|----------|--------|-----------------|
| SQLite | QSQLITE | File-based |
| MySQL | QMYSQL | Server-based |
| PostgreSQL | QPSQL | Server-based |

### Connection Parameters

#### SQLite

| Field | Description | Required |
|-------|-------------|----------|
| Database Name | Path to the `.db`, `.sqlite`, or `.sqlite3` file | Yes |

- Use **Choose SQLite file** to select a database file
- No host, username, or password required

#### MySQL / PostgreSQL

| Field | Description | Required |
|-------|-------------|----------|
| Host Name | Server hostname or IP address | Yes |
| Database Name | Name of the database | Yes |
| User Name | Database username | Yes |
| Password | Database password | Yes |

## Browsing Tables

Once connected to a database:

1. The **Tables** panel displays the available tables
2. Click a table name to put a SELECT statement for it in the active SQL editor
3. Click **Execute** to display the query results below the editor
4. Use **Refresh** in the toolbar to reload the table list

Results are limited to the first 1,000 rows.

## Executing SQL Queries

### Adding a Query Tab

1. Click **New SQL** in the toolbar
2. A new SQL tab opens

### Running a Query

1. Enter your SQL statement in the text editor
2. Click the **Execute** button
3. Results are displayed in the table view below

### Managing Tabs

- Tabs can be closed by clicking the **×** button on each tab
- Multiple query tabs can be open simultaneously
- Each tab contains a SQL editor and its query results

### Example Queries

```sql
SELECT * FROM users;
```

```sql
SELECT name, email FROM users WHERE active = 1 ORDER BY name;
```

```sql
INSERT INTO users (name, email) VALUES ('John Doe', 'john@example.com');
```

## Connection History

- Connections are automatically saved to history when established
- Up to **10** most recent connections are stored
- Duplicate connections (same database, type, and host) are moved to the top instead of duplicated
- Connection history persists between application sessions
- History entries display as:
  - SQLite: `SQLite: filename.db`
  - MySQL/PostgreSQL: `Type: user@host/database`

## Tips and Best Practices

### Connection Management

- Use SQLite for local development and testing
- Connection state is preserved when switching between tools — you don't need to reconnect each time
- Close unused connections to free resources

### Query Writing

- Start with simple `SELECT` queries to explore data
- Use table browsing to understand the schema before writing queries
- Be cautious with `UPDATE`, `DELETE`, and `DROP` statements — there is no undo

### Performance

- For large tables, use `LIMIT` clauses to avoid loading excessive data
- Use `WHERE` clauses to filter results

## Troubleshooting

### Connection Failed

1. **SQLite**: Verify the file path exists and the file is a valid SQLite database
2. **MySQL/PostgreSQL**: Check hostname, credentials, and that the server is running
3. **Driver not found**: Ensure the Qt SQL driver plugin is installed for your database type

### No Tables Shown

1. **Check connection**: Verify the database is connected (toolbar buttons should be enabled)
2. **Empty database**: The database may not contain any tables
3. **Refresh**: Click the Refresh button to reload the table list

### Query Errors

1. **Syntax errors**: Check your SQL syntax for the specific database dialect
2. **Table not found**: Verify the table name matches your schema definition (case handling depends on database/configuration)
3. **Permission denied**: Ensure your database user has the required permissions

### Database Type Not Supported

If you see "Database type not supported", the selected database driver may not be available in your Qt installation. Check the [Installation Guide](../getting-started/installation.md) for required dependencies.

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
- [Architecture](../development/architecture.md) - System architecture
