#include "k35190/m35190.h"
QVector<double> m35190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
