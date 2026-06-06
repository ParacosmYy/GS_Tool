#include "m37232/m37232.h"
QVector<double> m37232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
