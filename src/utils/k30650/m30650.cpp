#include "k30650/m30650.h"
QVector<double> m30650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
