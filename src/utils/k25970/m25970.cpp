#include "k25970/m25970.h"
QVector<double> m25970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
