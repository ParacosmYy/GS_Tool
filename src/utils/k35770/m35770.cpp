#include "k35770/m35770.h"
QVector<double> m35770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
