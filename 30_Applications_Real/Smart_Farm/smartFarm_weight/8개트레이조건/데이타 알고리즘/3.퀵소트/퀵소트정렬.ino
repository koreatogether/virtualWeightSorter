void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high);

        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

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

    quickSort(data, 0, size - 1);

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

/*

또 다른 퀵소트 코드 예제


void quickSort(int arr[], int low, int high) {
  if (low < high) {
    int pi = partition(arr, low, high);

    quickSort(arr, low, pi - 1);
    quickSort(arr, pi + 1, high);
  }
}

int partition(int arr[], int low, int high) {
  int pivot = arr[high];
  int i = (low - 1);

  for (int j = low; j <= high - 1; j++) {
    if (arr[j] < pivot) {
      i++;
      int temp = arr[i];
      arr[i] = arr[j];
      arr[j] = temp;
    }
  }
  int temp = arr[i + 1];
  arr[i + 1] = arr[high];
  arr[high] = temp;
  return (i + 1);
}

void setup() {
  Serial.begin(9600);

  int data[] = {64, 25, 12, 22, 11};
  int size = sizeof(data) / sizeof(data[0]);

  Serial.println("Unsorted array:");
  for (int i = 0; i < size; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println();

  quickSort(data, 0, size - 1);

  Serial.println("Sorted array:");
  for (int i = 0; i < size; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  // Do nothing here
}