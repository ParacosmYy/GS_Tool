#include "h9307/m9307.h"
QVector<double> m9307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
