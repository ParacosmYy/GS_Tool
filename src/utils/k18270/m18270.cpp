#include "k18270/m18270.h"
QVector<double> m18270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
