#include "e8604/m8604.h"
QVector<double> m8604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
