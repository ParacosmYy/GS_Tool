#include "k34030/m34030.h"
QVector<double> m34030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
