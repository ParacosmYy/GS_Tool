#include "k18850/m18850.h"
QVector<double> m18850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
