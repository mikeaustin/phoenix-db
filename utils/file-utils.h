uint8_t *openTable(const char *filename) {
    const size_t FILE_SIZE = 4096;

    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        std::perror("Error opening/creating file");

        exit(1);
    }

    if (ftruncate(fd, FILE_SIZE) == -1) {
        std::perror("Error setting file size");
        close(fd);

        exit(1);
    }

    void *map = mmap(nullptr, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (map == MAP_FAILED) {
        std::perror("Error mapping the file");
        close(fd);

        exit(1);
    }

    close(fd);

    uint8_t *items = static_cast<uint8_t *>(map);

    return items;
}
