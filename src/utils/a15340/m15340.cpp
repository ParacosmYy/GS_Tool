#include "a15340/m15340.h"
QVector<double> m15340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
