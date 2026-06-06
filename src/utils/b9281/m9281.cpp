#include "b9281/m9281.h"
QVector<double> m9281::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
