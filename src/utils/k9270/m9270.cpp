#include "k9270/m9270.h"
QVector<double> m9270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
