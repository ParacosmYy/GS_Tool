#include "k35050/m35050.h"
QVector<double> m35050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
