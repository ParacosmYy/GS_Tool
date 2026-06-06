#include "c18102/m18102.h"
QVector<double> m18102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
