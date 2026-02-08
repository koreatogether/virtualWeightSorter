void setup()
{
    Serial.begin(9600);

    int data[] = {64, 25, 12, 22, 11};
    int size = sizeof(data) / sizeof(data[0]);

    Serial.println("Unsorted array:");
    for (int i = 0; i < size; i++)
    {
        Serial.print(data[i]);
        Serial.print(" ");
    }
    Serial.println();

    selectionSort(data, size);

    Serial.println("Sorted array:");
    for (int i = 0; i < size; i++)
    {
        Serial.print(data[i]);
        Serial.print(" ");
    }
    Serial.println();
}

void loop()
{
    // Do nothing here
}

void selectionSort(int arr[], int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int minIndex = i;
        for (int j = i + 1; j < n; j++)
        {
            if (arr[j] < arr[minIndex])
            {
                minIndex = j;
            }
        }
        // Swap the found minimum element with the first element
        int temp = arr[minIndex];
        arr[minIndex] = arr[i];
        arr[i] = temp;
    }
}