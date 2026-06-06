#include "f28365/m28365.h"
QVector<double> m28365::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
