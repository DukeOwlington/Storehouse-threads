#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <time.h>
#include <pthread.h>
#define BUYER_NUM 3

/*global variables for threads*/
int storehouse[5];
int buyer_demands[3];
pthread_mutex_t store_lock[5];
bool release_condition;

/* Initializers and print functions */
void InitializeStores() {
  int i;
  srand(time(NULL));
  for (i = 0; i < 5; i++) {
    storehouse[i] = (rand() % 499) + 1001;
  }
}

void InitializeDemands() {
  int i;
  for (i = 0; i < 3; i++) {
    buyer_demands[i] = (rand() % 499) + 2501;
  }
}

void PrintStoreInfo() {
  int i;
  for (i = 0; i < 5; i++) {
    printf("Storehouse %d is loaded with %d goods\n", i, storehouse[i]);
  }
}

void PrintDemandsInfo() {
  int i;
  for (i = 0; i < 3; i++) {
    printf("Demands of buyer %d is %d\n", i, buyer_demands[i]);
  }
}

/*Supply loader thread*/
void *StoreLoader() {
  int chosen_store;
  int goods_amount;
  srand(time(NULL));

  while (!release_condition) {
    goods_amount = (rand() % 99) + 201;
    chosen_store = (rand() % 5);

    pthread_mutex_lock(&store_lock[chosen_store]);
    storehouse[chosen_store] += goods_amount;
    printf("Storehouse %d is loaded with %d goods. Current amount: %d\n",
      chosen_store, goods_amount, storehouse[chosen_store]);
    pthread_mutex_unlock(&store_lock[chosen_store]);
    puts("Store loader is going to sleep for 2 sec");
    sleep(2);
    puts("Store loader has awoken");
  }
  pthread_exit(NULL);
}

/*if buyer wants more than he needs
 *and storehouse have more than he needs*/
bool IsOversupply(int goods_amount, int buyer_num, int chosen_store) {
  if ((goods_amount > buyer_demands[buyer_num])
      && (storehouse[chosen_store] > buyer_demands[buyer_num]))
    return true;
  else
    return false;
}

/*buyers thread*/
void *BuyGoods(void *chosen_buyer) {
  int chosen_store;
  int goods_amount;
  int goods_bought;
  int buyer_num = *(int *)chosen_buyer;
  srand(time(NULL));

  while (buyer_demands[buyer_num] != 0) {
    goods_amount = (rand() % 99) + 401;
    chosen_store = (rand() % 5);

    /*if the storehouse is busy by loader*/
    if(pthread_mutex_trylock(&store_lock[chosen_store]) != 0)
      continue;
    /*if goods amount that buyer wants is more than left in the storehouse*/
    if (goods_amount > storehouse[chosen_store]) {
      /*if buyer demand is less then left in the storehouse*/
      if (IsOversupply(goods_amount, buyer_num, chosen_store)) {
        goods_bought = buyer_demands[buyer_num];
        buyer_demands[buyer_num] = 0;
      }
      else {
        goods_bought = storehouse[chosen_store];
        buyer_demands[buyer_num] -= storehouse[chosen_store];
      }
      storehouse[chosen_store] = 0;
    }
    else {
      if (IsOversupply(goods_amount, buyer_num, chosen_store)) {
        goods_bought = buyer_demands[buyer_num];
        buyer_demands[buyer_num] = 0;
      }
      else {
        goods_bought = goods_amount;
        buyer_demands[buyer_num] -= goods_amount;
      }
      storehouse[chosen_store] -= goods_amount;
    }
    printf("Buyer %d bought %d goods from store %d. Demands: %d\n",
      buyer_num, goods_bought, chosen_store, buyer_demands[buyer_num]);
    printf("Store %d goods left: %d\n", chosen_store, storehouse[chosen_store]);
    pthread_mutex_unlock(&store_lock[chosen_store]);
    printf("Buyer %d is going to sleep\n", buyer_num);
    sleep(1);
    printf("Buyer %d has awoken\n", buyer_num);
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int i;
  int buyer_num[3];
  pthread_t loader;
  pthread_t buyer[BUYER_NUM];
  pthread_attr_t attr;

  release_condition = false;

  /*initialize storehouses and buyers demands*/
  InitializeStores();
  InitializeDemands();


  /*print initial values*/
  printf("===INITIAL INFO===\n");
  PrintStoreInfo();
  PrintDemandsInfo();
  printf("==================\n");

  /*initialize mutexes locks*/
  for (i = 0; i < 5; i++) {
    pthread_mutex_init(&store_lock[i], NULL);
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&loader, NULL, StoreLoader, NULL);

  for (i = 0; i < 3; i++) {
    buyer_num[i] = i;
    pthread_create(&buyer[i], &attr, BuyGoods, (void *)&buyer_num[i]);
  }

  pthread_attr_destroy(&attr);

  /*wait buyers*/
  for(i = 0; i < 3; i++) {
    pthread_join(buyer[i], NULL);
  }
  /*release condition for loader*/
  release_condition = true;
  /*wait loader*/
  pthread_join(loader, NULL);
  pthread_mutex_destroy(store_lock);
  /*output what's left*/
  printf("===SUMMARY===\n");
  PrintStoreInfo();
  PrintDemandsInfo();
  pthread_exit(NULL);
  printf("=============\n");
}
