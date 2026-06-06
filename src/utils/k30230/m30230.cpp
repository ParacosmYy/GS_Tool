#include "k30230/m30230.h"
QVector<double> m30230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
