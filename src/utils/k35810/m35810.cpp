#include "k35810/m35810.h"
QVector<double> m35810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
