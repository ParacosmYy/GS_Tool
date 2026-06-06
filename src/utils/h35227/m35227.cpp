#include "h35227/m35227.h"
QVector<double> m35227::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
