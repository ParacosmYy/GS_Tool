#include "h8947/m8947.h"
QVector<double> m8947::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
