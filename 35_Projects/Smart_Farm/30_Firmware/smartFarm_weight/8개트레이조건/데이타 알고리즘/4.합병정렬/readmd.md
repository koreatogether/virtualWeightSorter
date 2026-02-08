합병 정렬 설명서
안녕하세요, 친구들! 오늘은 합병 정렬이라는 정렬 방법에 대해 배워볼 거예요. 합병 정렬은 숫자들을 정렬하는 아주 똑똑한 방법이에요. 자, 그럼 합병 정렬이 어떻게 작동하는지 알아볼까요?

합병 정렬이란?
합병 정렬은 배열(숫자들이 모여 있는 곳)을 정렬하는 방법 중 하나예요. 이 방법은 배열을 반으로 나누고, 각각을 정렬한 다음 다시 합쳐서 전체를 정렬하는 방식이에요.

합병 정렬의 단계
분할: 배열을 반으로 나눠요.
정렬: 나눈 부분 배열을 각각 정렬해요.
합병: 정렬된 부분 배열을 합쳐서 하나의 정렬된 배열로 만들어요.
예시
예를 들어, 우리가 {64, 25, 12, 22, 11}이라는 숫자들을 정렬한다고 해볼게요.

분할: 배열을 반으로 나눠요. {64, 25, 12}와 {22, 11}로 나눠요.
정렬: 각각의 부분 배열을 정렬해요.
{64, 25, 12}를 다시 반으로 나눠요. {64, 25}와 {12}로 나눠요.
{64, 25}를 다시 나눠요. {64}와 {25}로 나눠요.
이제 {64}와 {25}를 정렬해요. {25, 64}가 돼요.
{25, 64}와 {12}를 합쳐서 정렬해요. {12, 25, 64}가 돼요.
{22, 11}을 나눠요. {22}와 {11}로 나눠요.
이제 {22}와 {11}을 정렬해요. {11, 22}가 돼요.
합병: 정렬된 부분 배열을 합쳐요. {12, 25, 64}와 {11, 22}를 합쳐서 정렬해요. {11, 12, 22, 25, 64}가 돼요.

아두이노 코드

```c++
void merge(int arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    int L[n1], R[n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
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

    mergeSort(data, 0, size - 1);

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


```