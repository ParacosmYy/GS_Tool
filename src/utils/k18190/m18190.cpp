#include "k18190/m18190.h"
QVector<double> m18190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
