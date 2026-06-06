#include "a25340/m25340.h"
QVector<double> m25340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
