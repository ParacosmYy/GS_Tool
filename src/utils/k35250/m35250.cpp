#include "k35250/m35250.h"
QVector<double> m35250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
