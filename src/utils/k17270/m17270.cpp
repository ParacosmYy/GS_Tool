#include "k17270/m17270.h"
QVector<double> m17270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
