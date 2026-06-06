#include "k30850/m30850.h"
QVector<double> m30850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
