#include "k35270/m35270.h"
QVector<double> m35270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
