LPDBM
==
LPDBM stands for Linked Physical Database Model

The model allow you to create a database and to add, modify, read and remove data from it.
When data is removed from the database, then the database won't get smaller, it will only mark the data as deleted and if data is added, it will override the deleted marked data.

I hope this model helps you to simply store data for you project.

## Database model
```
Start position
Removed start position
Size
  Datasize
  Data
  Position of remaining data
```

The DBMS use the **Start position** to get and set the entry point for the database.
**Datasize**, **Data** and **Position of remaining data** difines a datablock.

## Code
You can find some code in the folders. I just implementetd the functionality I need for the system and langugage I need. More informations about the code are in the folders.
