#include "p25815/m25815.h"
QVector<double> m25815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
