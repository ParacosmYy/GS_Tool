#include "m35472/m35472.h"
QVector<double> m35472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
