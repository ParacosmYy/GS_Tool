#include "e10604/m10604.h"
QVector<double> m10604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
