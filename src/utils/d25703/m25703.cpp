#include "d25703/m25703.h"
QVector<double> m25703::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
