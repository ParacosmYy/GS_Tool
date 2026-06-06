#include "e15604/m15604.h"
QVector<double> m15604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
