# Database Management

Connect to SQL databases and execute queries directly from DevTools.

## Overview

The Database Management tool provides a graphical interface for connecting to SQL databases, browsing table structures, and executing queries. It supports SQLite, MySQL, and PostgreSQL databases.

## Accessing the Tool

1. Launch DevTools
2. Click **DB** in the side menu

On first access (with no connection history), the Connection Selector dialog opens automatically.

## Interface Overview

The Database Management interface consists of:

1. **Toolbar**: Action buttons for database operations
   - **Refresh** button: Reload the table list
   - **DB接続設定** button: Open the connection settings dialog
   - **SQL** button: Add a new SQL query tab
2. **Tables** panel (left): Displays a list of tables in the connected database
3. **Query** panel (right): Tabbed area for SQL queries and table views

> **Note:** The Refresh and SQL buttons are disabled until a database connection is established.

## Connecting to a Database

### Using Connection History

When you click **DB接続設定**, the Connection Selector dialog opens:

1. Select a previously used connection from the **Connection History** list
2. The connection is established automatically
3. The dialog closes and tables are loaded

You can also:
- Click the **Delete** button next to a connection to remove it from history
- Click **New Connection** to create a new connection
- Click **Close** to dismiss the dialog

### Creating a New Connection

1. Click **DB接続設定** in the toolbar
2. Click **New Connection** in the Connection Selector
3. Select the **Database Type** from the dropdown
4. Fill in the required connection fields (see below)
5. Click **Connect**
6. A success message confirms the connection

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

- Use the **Browse...** button to select a database file via file dialog
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

1. The **Tables** panel displays all tables in the database
2. Click a table name to open it in a new tab
3. The table data is displayed in a sortable table view
4. Use the **Refresh** button within the tab to reload table data
5. Use the toolbar **Refresh** button to reload the table list

If you click a table that already has an open tab, the existing tab is selected instead of creating a duplicate.

## Executing SQL Queries

### Adding a Query Tab

1. Click the **SQL** button in the toolbar
2. A new query tab opens (named "Query 1", "Query 2", etc.)

### Running a Query

1. Enter your SQL statement in the text editor
2. Click the **実行** (Execute) button
3. Results are displayed in the table view below

### Managing Tabs

- Tabs can be closed by clicking the **×** button on each tab
- Multiple query tabs can be open simultaneously
- Table view tabs and query tabs coexist in the same tab area

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
2. **Table not found**: Verify the table name matches exactly (case-sensitive)
3. **Permission denied**: Ensure your database user has the required permissions

### Database Type Not Supported

If you see "Database type not supported", the selected database driver may not be available in your Qt installation. Check the [Installation Guide](../getting-started/installation.md) for required dependencies.

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
- [Architecture](../development/architecture.md) - System architecture
