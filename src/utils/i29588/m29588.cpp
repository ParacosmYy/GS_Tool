#include "i29588/m29588.h"
QVector<double> m29588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
