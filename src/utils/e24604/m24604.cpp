#include "e24604/m24604.h"
QVector<double> m24604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
