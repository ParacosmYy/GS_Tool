#include "g28526/m28526.h"
QVector<double> m28526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
