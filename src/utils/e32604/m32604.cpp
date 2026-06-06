#include "e32604/m32604.h"
QVector<double> m32604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
