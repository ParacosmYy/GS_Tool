#include "c8062/m8062.h"
QVector<double> m8062::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
