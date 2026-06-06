#include "d9703/m9703.h"
QVector<double> m9703::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
