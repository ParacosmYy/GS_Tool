#include "e28604/m28604.h"
QVector<double> m28604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
