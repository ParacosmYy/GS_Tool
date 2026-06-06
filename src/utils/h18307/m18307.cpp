#include "h18307/m18307.h"
QVector<double> m18307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
