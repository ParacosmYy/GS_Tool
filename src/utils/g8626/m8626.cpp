#include "g8626/m8626.h"
QVector<double> m8626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
