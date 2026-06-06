#include "j8069/m8069.h"
QVector<double> m8069::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
