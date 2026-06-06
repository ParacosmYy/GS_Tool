#include "s9878/m9878.h"
QVector<double> m9878::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
