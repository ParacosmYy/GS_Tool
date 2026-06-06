#include "k21030/m21030.h"
QVector<double> m21030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
