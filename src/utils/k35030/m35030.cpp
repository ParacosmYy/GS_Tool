#include "k35030/m35030.h"
QVector<double> m35030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
