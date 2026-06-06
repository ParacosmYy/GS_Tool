#include "k35870/m35870.h"
QVector<double> m35870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
