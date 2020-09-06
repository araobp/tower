#include <mutex>

void getJpegImage(char *jpegArray, size_t *size, bool *personDetected);
void drain(int jpegQuality, bool proximitySensing, bool equalize, bool show, bool record, int angleOfView, bool verbose);
int process(float accThres, bool verbose);
void stopProcess();
extern std::mutex copyLock;

