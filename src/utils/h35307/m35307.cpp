#include "h35307/m35307.h"
QVector<double> m35307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
