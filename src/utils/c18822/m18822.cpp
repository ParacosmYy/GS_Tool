#include "c18822/m18822.h"
QVector<double> m18822::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
