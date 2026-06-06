#include "c32822/m32822.h"
QVector<double> m32822::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
