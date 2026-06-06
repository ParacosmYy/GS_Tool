#include "k25930/m25930.h"
QVector<double> m25930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
