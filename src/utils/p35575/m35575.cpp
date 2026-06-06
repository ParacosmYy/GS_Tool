#include "p35575/m35575.h"
QVector<double> m35575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
