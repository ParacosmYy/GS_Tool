#include "n8293/m8293.h"
QVector<double> m8293::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
