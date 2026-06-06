#include "e8544/m8544.h"
QVector<double> m8544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
